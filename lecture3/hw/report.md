# 项目理解报告

请尽量使用自己的语言回答以下问题。可以引用少量关键代码或伪代码，但不要只粘贴实现。
完成一节后删除该节末尾的待填写标记；本地检查会拒绝仍有未完成章节的报告。

## 1. 图像生命周期与所有权

解释本项目中图像源为什么会复用缓冲区，以及 `cv::Mat` 的普通复制对底层像素数据
意味着什么。说明你的修改让一个 `Frame` 在进入队列后拥有什么，并解释为何后续读取
不会再改变它。

原来的代码只是让frame和buffer指向了同一个内容
我的修改做到了把原来的buffer的内容额外给了frame一份，这样后续对buffer的修改不会再改变frame指向的内容了。

## 2. 并发处理与恰好一次

结合 `BlockingQueue` 的 `push`、`pop` 和 `close` 行为，解释多个 worker 如何分工。
为什么你的实现既不会漏掉已经入队的帧，也不会重复处理同一帧？输入耗尽时，正在等待
以及仍在处理数据的 worker 分别会怎样？

producer 通过 push() 将每个 Frame 放入队列，多个 worker 通过 pop() 竞争或得任务。BlockingQueue 对队列操作进行了同步，所以一次 pop() 会独占地取出并删除一个元素，一个已经被某个 worker 取走的 Frame 不会再次被另一个 worker 取得。
当生产者读完所有输入后调用 close()。关闭队列表示以后不会再有新的数据加入，但已经进入队列的数据仍然可以继续被 worker 取出。因此 worker 会先处理完队列中的剩余 Frame。当队列已经关闭且为空时，pop() 返回失败，worker 的循环结束。
我在 todo处 加入的 wait() 本身不负责分配任务，而是通过 join() 等待 producer 和所有 worker 真正结束。这样 Pipeline 不会在线程仍然访问其成员时被提前销毁，也能保证调用者在 wait() 返回时，已经入队的数据都处理完成。

## 3. 共享统计数据

指出哪些线程会读写 `Statistics`。解释原实现中的竞争为什么可能导致错误结果，并说明
你的同步方案提供了什么保证。还应说明取得快照时为什么是安全的。

pipeline的Frame的工作途中会读取该文件，并修改图片处理的数目内容。
我的同步方案加入了mutex锁，保证了多个frame不会同时都对统计数据+1,而是一个lock_guard 的作用域结束后才允许下一个。

## 4. 线程关闭协议

分别描述以下两条路径中的事件顺序，并解释为什么不会发生 `std::terminate`、悬空访问
或永久等待：

1. 调用者执行 `start()` 后显式调用 `wait()`；
2. 调用者执行 `start()` 后不调用 `wait()`，直接让 `Pipeline` 析构。

如果你的实现允许某个生命周期方法被重复调用，也请说明其行为；如果不允许，请说明前置条件。

1：
调用 start() 后，程序会启动一个 producer 线程和多个 worker 线程。producer 不断读取帧并通过：
queue_.push(frame) 把任务放入队列。读取结束以后，producer 调用：
queue_.close(),结束帧的读取过程，然后 producer 线程结束。
当调用者执行：pipeline.wait()，时，首先：
producer_.join()，等待 producer 执行完成。
这里 join() 只会让调用 wait() 的线程等待，并不会停止 worker，因此 worker 仍然可以继续从队列中取出和处理数据。
producer 结束后，wait() 再依次worker.join();
等待所有 worker 执行完成。
worker 会继续处理队列中剩余的数据。当队列已经关闭并且为空时：
while(queue_.pop(frame))这一句返回false，worker 退出循环并结束线程。
因此 wait() 返回时，producer 和所有 worker 都已经结束。

2：
我只是把wait()原样的放入了 Pipeline的析构函数中，最后会等到Pipeline的作用域完成后调用析构函数，析构函数内部依旧会等待所有的worker和producer执行相应的任务
因为有条件语句(xxx.jionable())的执行，
不用担心对一个 thread 进行了两次join，也就没有 std::terminate的发生，所以就算执行者再显示调用 wait(),我的析构函数也不会出问题，
执行者不调用时，我的析构函数起最后的保障。
因此，我的实现允许重复调用wait();


