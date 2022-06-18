# HuaweiSoftwareEliteChallenge_2022
## 2022华为软件精英挑战赛<br>
## 1.1 题目背景
某个时刻边缘节点为客户节点分配的带宽资源示意图如图示，
* （1）共有M个客户节点和N个边缘节点。 
* （2）在每个时刻，要决策如何把每个客户节点的带宽需求分配到边缘节点。
* （3）为了确保调度质量，每个客户节点的需求只能分配到满足QoS约束的边缘节点上，即：当客户节点和边缘节点之间的QoS小于“QoS上限”时，才会进行流量分配。
* （4）在每个时刻，每个边缘节点接收的带宽需求总和不能超过其带宽上限。
* （6）合理分配所有时刻的客户节点带宽需求，使得最终的带宽总成本尽量小。
![image](https://user-images.githubusercontent.com/54426524/174443291-4fc7ed9c-130d-4b9a-8dc1-3022cd30faac.png)
##1.2 技术路线
理论上，要求流分配的top5%成本不计算，则需要将5%成本尽量填满。要求排序后95%位置的成本最低，则理论上均匀分布的最大值（成本）最小，所以理论上需要将后95%做平均分配。理论分配模型如右图所示。
我们需要设计的效果如下表，不同边缘节点在不同时间下的流的分布情况，0代表流按照边缘节点的大小直接填满，o代表做后95%的均匀分布。
![image](https://user-images.githubusercontent.com/54426524/174443317-ac99bd49-bcc4-47ea-8999-726b22a34002.png)
![image](https://user-images.githubusercontent.com/54426524/174443339-e4b21f1c-5dca-4dcc-b66d-8697418eaa2d.png)
抽象边缘节点和用户节点，建立二分图，提出反馈机制，使得5%的节点被填满，95%的节点处于均匀分布。
##算法流程：
* （1）抽象数据，建立流、边缘节点、用户节点结构体，按照qos关系约束建立二分图（二分图包含的信息：每一个时刻流的总请求，被填满边缘节点的个数）；
* （2）采用反馈机制，使得剩余流的平均值一致，达到做均匀分布的目的。
![image](https://user-images.githubusercontent.com/54426524/174443357-d11ede61-8c65-4892-8422-3a3e87bed561.png)
* （3）采用了贪心算法，探索机制等算法均匀分配剩余的95%的节点。
##1.3 完成效果
![image](https://user-images.githubusercontent.com/54426524/174443365-00c539ba-aba8-453d-879e-c20fb4ae36af.png)
![image](https://user-images.githubusercontent.com/54426524/174443383-838d5e86-7ade-4370-9283-9065d13ecdef.png)
![image](https://user-images.githubusercontent.com/54426524/174443389-2a5354b4-363b-413e-bc21-c78f4363d73b.png)
![image](https://user-images.githubusercontent.com/54426524/174443393-9ca9e5b6-e4ad-4393-97e2-fb527e54bfdb.png)
![image](https://user-images.githubusercontent.com/54426524/174443397-449b641b-bc7c-4d46-b3f4-9c289a7a4b6c.png)
