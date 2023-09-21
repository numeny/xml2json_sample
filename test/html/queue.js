class Queue {
    constructor() {
      this.items = [];
    }
  
    // 入队操作，将元素添加到队列末尾
    enqueue(element) {
      this.items.push(element);
    }
  
    // 出队操作，移除并返回队列的第一个元素
    dequeue() {
      if (this.isEmpty()) {
        return null;
      }
      return this.items.shift();
    }
  
    // 返回队列的第一个元素，但不移除它
    front() {
      if (this.isEmpty()) {
        return null;
      }
      return this.items[0];
    }
  
    // 检查队列是否为空
    isEmpty() {
      return this.items.length === 0;
    }
  
    // 返回队列的长度
    size() {
      return this.items.length;
    }
  
    // 清空队列
    clear() {
      this.items = [];
    }
  }
  