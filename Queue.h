template <typename T>
class Queue {
public:
    Queue(){
        front = nullptr;
        back = nullptr;
        length = 0;
    }
    ~Queue(){
        clear();
    }
    void enqueue(T data){
        if(isEmpty()){
            front = new Node(data);
            back = front;
        }else{
            back->next = new Node(data);
            back = back->next;
        }
        length++;
    }
    T dequeue(){
        if(isEmpty()){
            return T();
        }
        Node* temp = front;
        T data = temp->data;
        front = front->next;
        delete temp;
        length--;
        return data;
    }
    bool isEmpty(){
        return length == 0;
    }
    int size(){
        return length;
    }
    void clear(){
        while(!isEmpty()){
            dequeue();
        }
    }
private:
    struct Node {
        T data;
        Node* next;
        Node(const int& data){
          this->data = data;
        }
    };
    Node* front;
    Node* back;
    int length;
};