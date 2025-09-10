#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import re
import threading
import os
import time
import logging
import requests
from lxml import etree
from concurrent.futures import ThreadPoolExecutor

# 配置日志
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler("crawler.log"),
        logging.StreamHandler()
    ]
)

# 配置项

START_URL = "https://dytt8.com/index.htm"
HOST = "https://dytt8.com"
# 可以根据需要修改存储路径
STORAGE_PATH = "D:/电影资源/"
# 控制并发线程数
MAX_WORKERS = 5
# 控制爬取间隔（秒）
CRAWL_INTERVAL = 1
# 超时时间（秒）
TIMEOUT = 30

# 用户代理池
USER_AGENTS = [
    'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36',
    'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36',
    'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Firefox/89.0 Safari/537.36',
    'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.1.1 Safari/605.1.15'
]

class MovieCrawler:
    def __init__(self):
        # 使用线程安全的集合存储已爬取的URL
        self.crawled_urls = set()
        self.url_lock = threading.Lock()
        # 创建线程池
        self.executor = ThreadPoolExecutor(max_workers=MAX_WORKERS)
        # 添加标志控制是否可以提交新任务
        self.shutdown_flag = False
        self.shutdown_lock = threading.Lock()
        # 确保存储目录存在
        self._ensure_dir_exists(STORAGE_PATH)
        
    def _ensure_dir_exists(self, dir_path):
        """确保目录存在，如果不存在则创建"""
        try:
            if not os.path.exists(dir_path):
                os.makedirs(dir_path)
                logging.info(f"创建目录成功: {dir_path}")
        except Exception as e:
            logging.error(f"创建目录失败 {dir_path}: {str(e)}")
            # 如果是根目录创建失败，尝试使用当前目录作为备选
            global STORAGE_PATH  # 在使用前声明全局变量
            if dir_path == STORAGE_PATH:
                # 先创建新的存储路径
                new_storage_path = os.path.join(os.getcwd(), "电影资源")
                logging.warning(f"尝试使用当前目录作为备选: {new_storage_path}")
                # 更新全局变量
                STORAGE_PATH = new_storage_path
                self._ensure_dir_exists(STORAGE_PATH)
    
    def _is_url_crawled(self, url):
        """检查URL是否已经被爬取过"""
        with self.url_lock:
            return url in self.crawled_urls
    
    def _mark_url_as_crawled(self, url):
        """标记URL为已爬取"""
        with self.url_lock:
            self.crawled_urls.add(url)
    
    def _get_page(self, url):
        """获取页面内容，带重试机制"""
        retries = 3
        while retries > 0:
            try:
                # 随机选择一个用户代理
                headers = {
                    'User-Agent': USER_AGENTS[hash(url) % len(USER_AGENTS)],
                    'Referer': HOST,
                    'Accept-Language': 'zh-CN,zh;q=0.9',
                    'Accept-Encoding': 'gzip, deflate, br',
                    'Connection': 'keep-alive'
                }
                
                logging.info(f"正在请求: {url}")
                response = requests.get(url, headers=headers, timeout=TIMEOUT)
                response.raise_for_status()  # 如果状态码不是200，抛出异常
                
                # 智能处理编码问题
                # 优先检查响应头中的编码
                if response.encoding:
                    logging.debug(f"响应头中的编码: {response.encoding}")
                
                # 尝试多种编码方式
                encodings = ['gbk', 'utf-8', 'gb2312', 'iso-8859-1']
                content = None
                
                for encoding in encodings:
                    try:
                        response.encoding = encoding
                        content = response.text
                        # 检查是否存在乱码（简单判断）
                        if '�' not in content:
                            logging.debug(f"使用编码 {encoding} 解析成功")
                            break
                    except Exception as e:
                        logging.debug(f"编码 {encoding} 解析失败: {str(e)}")
                        continue
                
                if content is None:
                    # 如果所有编码都失败，使用默认解码
                    content = response.text
                    logging.warning(f"所有编码尝试失败，使用默认解码")
                
                return content
                      
            except requests.exceptions.RequestException as e:
                retries -= 1
                logging.error(f"请求失败 {url}: {str(e)}. 剩余重试次数: {retries}")
                if retries > 0:
                    time.sleep(2)  # 重试前等待2秒
                else:
                    logging.error(f"请求失败且重试次数用完: {url}")
                    return None
    
    def crawl_source_page(self, url, file_dir, filename):
        """爬取电影详情页，保存页面URL"""
        if self._is_url_crawled(url):
            return
            
        # 控制爬取速率
        time.sleep(CRAWL_INTERVAL)
        
        # 不需要实际获取页面内容，只需要保存URL
        self._mark_url_as_crawled(url)
        
        try:
            # 确保文件目录存在
            self._ensure_dir_exists(file_dir)
            
            # 保存电影详情页URL到文件
            file_path = os.path.join(file_dir, f"{filename}_url.txt")
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(f"电影标题: {filename}\n")
                f.write(f"详情页URL: {url}\n")
                f.write(f"访问时间: {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
            
            logging.info(f"已保存电影详情页URL到 {file_path}")
            
        except Exception as e:
            logging.error(f"处理页面 {url} 时出错: {str(e)}")
    
    def crawl_list_page(self, index_url, file_dir):
        """爬取分类列表页"""
        if self._is_url_crawled(index_url):
            return
            
        # 控制爬取速率
        time.sleep(CRAWL_INTERVAL)
        
        page = self._get_page(index_url)
        if not page:
            return
        
        self._mark_url_as_crawled(index_url)
        logging.info(f"正在解析分类页: {index_url}")
        
        try:
            tree = etree.HTML(page)
            nodes = tree.xpath("//div[@class='co_content8']//a")
            
            for node in nodes:
                try:
                    url = node.xpath("@href")[0]
                    
                    if re.match(r'^/', url):
                        # 非分页地址 - 电影详情页
                        full_url = HOST + url
                        if not self._is_url_crawled(full_url):
                            # 清理文件名中的非法字符
                            try:
                                filename = node.xpath("text()")[0]
                                # 移除或替换文件名中的非法字符
                                filename = re.sub(r'[\\/:*?"<>|]', ' ', filename)
                                # 去除首尾空格并限制长度
                                filename = filename.strip()[:100]  # 限制文件名长度
                                
                                # 检查是否可以提交新任务
                                with self.shutdown_lock:
                                    if not self.shutdown_flag:
                                        try:
                                            # 使用线程池提交任务
                                            self.executor.submit(
                                                self.crawl_source_page, full_url, file_dir, filename
                                            )
                                        except Exception as e:
                                            logging.error(f"提交任务到线程池失败: {str(e)}")
                                    else:
                                        logging.warning(f"线程池已关闭，跳过提交任务: {full_url}")
                            except:
                                continue
                    else:
                        # 分页地址 - 递归爬取
                        if not url.startswith('http'):
                            # 构建完整的分页URL
                            index = index_url.rfind("/")
                            if index != -1:
                                base_url = index_url[:index + 1]
                                page_url = base_url + url
                                
                                if not self._is_url_crawled(page_url):
                                    # 使用线程池提交任务
                                    self.executor.submit(
                                        self.crawl_list_page, page_url, file_dir
                                    )
                except Exception as e:
                    logging.error(f"处理链接时出错: {str(e)}")
                    continue
        except Exception as e:
            logging.error(f"解析列表页 {index_url} 时出错: {str(e)}")
    
    def crawl_index_page(self, start_url):
        """爬取网站首页，获取所有分类"""
        logging.info(f"开始爬取首页: {start_url}")
        
        page = self._get_page(start_url)
        if not page:
            logging.error("无法获取首页内容，爬虫终止")
            return
        
        # 保存首页内容到文件，便于分析网站结构
        try:
            with open("index_debug.html", "w", encoding="utf-8") as f:
                f.write(page)
            logging.info("首页内容已保存到index_debug.html文件，便于分析")
        except Exception as e:
            logging.warning(f"保存首页内容失败: {str(e)}")
            
        try:
            tree = etree.HTML(page)
            
            # 尝试多种XPath选择器来适配可能变化的网站结构
            xpath_patterns = [
                "//div[@id='menu']//a",          # 原始选择器
                "//div[@class='menu']//a",       # 可能的类名变化
                "//ul[@class='menu']//a",        # 可能的标签变化
                "//div[contains(@id, 'menu')]//a",  # 模糊匹配
                "//div[contains(@class, 'menu')]//a", # 模糊匹配
                "//a[contains(@href, '/html/')]"   # 直接匹配分类链接模式
            ]
            
            all_nodes = []
            for pattern in xpath_patterns:
                nodes = tree.xpath(pattern)
                if nodes:
                    logging.info(f"使用选择器 '{pattern}' 找到 {len(nodes)} 个链接")
                    # 合并结果并去重
                    all_nodes.extend(nodes)
            
            # 去重
            unique_nodes = []
            seen_hrefs = set()
            for node in all_nodes:
                try:
                    href = node.xpath("@href")[0]
                    if href not in seen_hrefs:
                        seen_hrefs.add(href)
                        unique_nodes.append(node)
                except:
                    continue
            
            logging.info(f"总共找到 {len(unique_nodes)} 个唯一的分类链接")
            
            for node in unique_nodes:
                try:
                    url = node.xpath("@href")[0]
                    # 只处理符合特定模式的分类URL
                    if re.match(r'/html/[A-Za-z0-9_/]+(?:index\.html)?', url):
                        full_url = HOST + url
                        if not self._is_url_crawled(full_url):
                            # 获取分类名称
                            try:
                                catalog = node.xpath("text()")[0]
                            except:
                                # 如果无法获取文本，使用URL的一部分作为分类名
                                catalog = url.split('/')[-2] if '/' in url else '未知分类'
                            
                            # 清理分类名称中的非法字符
                            catalog = re.sub(r'[\\/:*?"<>|]', ' ', catalog).strip()
                            
                            # 创建分类目录
                            category_dir = os.path.join(STORAGE_PATH, catalog)
                            self._ensure_dir_exists(category_dir)
                            
                            logging.info(f"开始爬取分类: {catalog} ({full_url})")
                            # 检查是否可以提交新任务
                            with self.shutdown_lock:
                                if not self.shutdown_flag:
                                    try:
                                        # 使用线程池提交任务
                                        self.executor.submit(
                                            self.crawl_list_page, full_url, category_dir
                                        )
                                    except Exception as e:
                                        logging.error(f"提交任务到线程池失败: {str(e)}")
                                else:
                                    logging.warning(f"线程池已关闭，跳过提交任务: {full_url}")
                except Exception as e:
                    logging.error(f"处理分类链接时出错: {str(e)}")
                    continue
            
            # 不在这里关闭线程池，避免影响后续任务提交
            logging.info("首页分类链接已全部提交到线程池")
            
        except Exception as e:
            logging.error(f"解析首页时出错: {str(e)}")

# 主函数
if __name__ == "__main__":
    crawler = None
    try:
        logging.info("电影天堂爬虫启动")
        crawler = MovieCrawler()
        crawler.crawl_index_page(START_URL)
        
        # 设置关闭标志，阻止新任务提交
        if crawler:
            with crawler.shutdown_lock:
                crawler.shutdown_flag = True
                logging.info("已设置关闭标志，不再接受新任务")
            # 等待线程池中的任务完成
            logging.info("等待所有爬取任务完成...")
            crawler.executor.shutdown(wait=True)
            logging.info("所有爬取任务已完成")
            
    except KeyboardInterrupt:
        logging.info("爬虫被用户中断")
    except Exception as e:
        logging.error(f"爬虫运行出错: {str(e)}")
    finally:
        logging.info("爬虫结束")





