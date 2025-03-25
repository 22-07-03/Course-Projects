#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

// Course class to store the course number and minimum passing grade.
class Course {
private:
    string CourseNum;
    char Grade;

public:
    Course(string c, char g) : CourseNum(c), Grade(g) {}

    string getCourseNum() const {
        return CourseNum;
    }

    char getGrade() const {
        return Grade;
    }

    bool operator<(const Course& other) const {
        return CourseNum < other.CourseNum;
    }

    friend class UDGraph;
};

// Singly Linked List Node (SLLNode) to represent each node in the adjacency list.
class SLLNode {
public:
    Course* course;
    SLLNode* next;

    SLLNode(Course* c) : course(c), next(nullptr) {}
};

// Singly Linked List (SLL) to represent the adjacency list for each course.
class SLL {
private:
    SLLNode* head;

public:
    SLL() : head(nullptr) {}

    // Insert a course in the adjacency list.
    void insert(Course* course) {
        SLLNode* newNode = new SLLNode(course);
        if (!head) {
            head = newNode;
        } else {
            SLLNode* temp = head;
            while (temp->next) {
                temp = temp->next;
            }
            temp->next = newNode;
        }
    }

    // Remove a course from the adjacency list.
    void remove(string courseNum) {
        if (!head) return;
        SLLNode* temp = head;
        SLLNode* prev = nullptr;

        while (temp && temp->course->getCourseNum() != courseNum) {
            prev = temp;
            temp = temp->next;
        }

        if (temp) {
            if (prev) {
                prev->next = temp->next;
            } else {
                head = temp->next;
            }
            delete temp;
        }
    }

    // Check if a course exists in the adjacency list.
    bool find(string courseNum) {
        SLLNode* temp = head;
        while (temp) {
            if (temp->course->getCourseNum() == courseNum) {
                return true;
            }
            temp = temp->next;
        }
        return false;
    }

    // Get all courses in the adjacency list.
    vector<Course*> getAllCourses() {
        vector<Course*> courses;
        SLLNode* temp = head;
        while (temp) {
            courses.push_back(temp->course);
            temp = temp->next;
        }
        return courses;
    }
};

// Undirected Graph (UDGraph) to represent the course prerequisite structure.
class UDGraph {
private:
    vector<pair<string, SLL>> Nodes; // Store the course and its adjacency list
    unsigned int totalnodes;

    // Find the adjacency list for a specific course.
    SLL* findCourse(string courseNum) {
        for (auto& node : Nodes) {
            if (node.first == courseNum) {
                return &node.second;
            }
        }
        return nullptr;
    }

public:
    UDGraph() : totalnodes(0) {}

    // Insert a prerequisite into the graph.
    void InsertPreq(string P1, char Grade, string C1) {
        if (P1 == "") {
            // If there's no prerequisite, just return
            return;
        }

        SLL* courseAdjList = findCourse(C1);
        if (!courseAdjList) {
            Nodes.push_back({C1, SLL()});
            courseAdjList = &Nodes.back().second;
        }

        if (!courseAdjList->find(P1)) {
            Course* preq = new Course(P1, Grade);
            courseAdjList->insert(preq);
        }
    }

    // Remove a prerequisite from the graph.
    void RemovePreq(string P1, string C1) {
        SLL* courseAdjList = findCourse(C1);
        if (courseAdjList) {
            courseAdjList->remove(P1);
        }
    }

    // Check if P1 is a prerequisite for C1.
    bool IsPreq(string P1, string C1) {
        SLL* courseAdjList = findCourse(C1);
        if (!courseAdjList) return false;

        if (courseAdjList->find(P1)) return true;

        // Recursively check if P1 is a prerequisite of prerequisites
        for (Course* preq : courseAdjList->getAllCourses()) {
            if (IsPreq(P1, preq->getCourseNum())) return true;
        }
        return false;
    }

    // List all immediate prerequisites of a course.
    vector<string> ListImmPres(string C1) {
        vector<string> result;
        SLL* courseAdjList = findCourse(C1);
        if (!courseAdjList) return result;

        for (Course* preq : courseAdjList->getAllCourses()) {
            result.push_back(preq->getCourseNum() + " " + preq->getGrade());
        }

        sort(result.begin(), result.end());
        return result;
    }

    // List all follow-on courses that can be taken after a course is completed.
    vector<string> ListFollow(string P1) {
        vector<string> result;
        for (auto& node : Nodes) {
            if (IsPreq(P1, node.first)) {
                result.push_back(node.first);
            }
        }

        sort(result.begin(), result.end());
        return result;
    }

    // Check if a student can take a course based on their current course list.
    bool OneCourseQuery(string C1, vector<Course> CList) {
        for (Course& preq : CList) {
            if (!IsPreq(preq.getCourseNum(), C1)) return false;
        }
        return true;
    }
};

// Main function to handle input/output and execute the queries.
int main() {
    UDGraph graph;
    int N;
    cin >> N;

    // Reading course data
    for (int i = 0; i < N; ++i) {
        string courseNum;
        int P;
        cin >> courseNum >> P;
        for (int j = 0; j < P; ++j) {
            string preq;
            char grade;
            cin >> preq >> grade;
            graph.InsertPreq(preq, grade, courseNum);
        }
    }

    // Handling queries
    string query;
    while (cin >> query) {
        if (query == "X") break;

        if (query == "I") {
            string P1, C1;
            char grade;
            cin >> P1 >> grade >> C1;
            graph.InsertPreq(P1, grade, C1);
        } else if (query == "R") {
            string P1, C1;
            cin >> P1 >> C1;
            graph.RemovePreq(P1, C1);
        } else if (query == "Q") {
            string P1, C1;
            cin >> P1 >> C1;
            cout << graph.IsPreq(P1, C1) << endl;
        } else if (query == "L") {
            string C1;
            cin >> C1;
            vector<string> immPres = graph.ListImmPres(C1);
            if (immPres.empty()) {
                cout << "CS0000" << endl;
            } else {
                for (string s : immPres) {
                    cout << s << " ";
                }
                cout << endl;
            }
        } else if (query == "F") {
            string C1;
            cin >> C1;
            vector<string> followCourses = graph.ListFollow(C1);
            if (followCourses.empty()) {
                cout << "CS0000" << endl;
            } else {
                for (string s : followCourses) {
                    cout << s << " ";
                }
                cout << endl;
            }
        } else if (query == "G") {
            string C1;
            int K;
            cin >> C1 >> K;
            vector<Course> CList;
            for (int i = 0; i < K; ++i) {
                string courseNum;
                char grade;
                cin >> courseNum >> grade;
                CList.push_back(Course(courseNum, grade));
            }
            cout << graph.OneCourseQuery(C1, CList) << endl;
        }
    }

    return 0;
}
