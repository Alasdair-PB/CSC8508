//
// Contributors: Alfie
//

#ifndef DUALTREE_H
#define DUALTREE_H



class DualTree {
private:

    enum Axis : unsigned char {
        x,
        y,
        z
    };

    struct Node {
        Node* less;
        Node* more;
        float division;
        Axis axis;
    };
};



#endif //DUALTREE_H
