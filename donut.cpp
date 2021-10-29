#include <iostream> 
#include <eigen3/Eigen/Dense>
#include <SFML/Graphics.hpp>

#include <math.h>

using namespace std;
using namespace Eigen;

const int step_c = 10;
const int step_t = 20;
const int modSize = step_c * step_t;
const float step = 5;
const float angle_step = M_PI/50;

const float width = 800;
const float height = 600;

//create a circle with radius r1 and of center {x,y,z}
//the circle rotate around the z-axis
Vector4f* create_circle(float r1, float x , float y, float z) {
    static Vector4f circle[step_c];
    float angle = 0;
    for(int i = 0; i < step_c; i++) {
        angle = 2*M_PI*i/step_c;
        circle[i](0) = x + r1*cos(angle);
        circle[i](1) = y + r1*sin(angle);
        circle[i](2) = 0.0f;
        circle[i](3) = 1.0f;
    }
    return circle;
}


//the rotation matrix will be around the y-axis (could've certainly done it with Eigen functions)
Matrix4f rotMaty(float angle) {
    Matrix4f rot;
    rot << cos(angle), 0.0f,             sin(angle), 0.0f,
            0.0f,       1.0f,             0.0f, 0.0f,
            -sin(angle), 0.0f,             cos(angle), 0.0f,
            0.0f,       0.0f,             0.0f, 1.0f;
    return rot;
}


//we copy-paste the function above for the x-axis
Matrix4f rotMatx(float angle) {
    Matrix4f rot;
    rot <<  1.0f,       0.0f,             0.0f, 0.0f,
            0.0f,       cos(angle),      -sin(angle), 0.0f,
            0.0f,       sin(angle),       cos(angle), 0.0f,
            0.0f,       0.0f,             0.0f, 1.0f;
    return rot;
}


//create a translation matrix
Matrix4f translateMatrix(float tx, float ty, float tz) {
    Matrix4f tr;
    tr << 1.0f, 0.0f, 0.0f, tx,
            0.0f, 1.0f, 0.0f, ty,
            0.0f, 0.0f, 1.0f, tz,
            0.0f, 0.0f, 0.0f, 1.0f;
    return tr;
}

//create the torus by creating multiple circle with a rotation around the y-axis
Vector4f* create_torus(float x, float y, float z, float r1, float r2) {
    static Vector4f torus[step_t*step_c];
    
    //create a basic cirle, we will rotate it to create the torus
    Vector4f* ref = create_circle(r1, x, y, z);
    
    //will apply translation and rotation transformations to the circle
    Matrix4f rot;
    Matrix4f tr = translateMatrix(r2, 0,0);

    //incrementation on the angle
    float angle = 0;
    for (int i = 0 ; i < step_t ; i++) {
        angle = 2*M_PI*i/step_t;
        rot = rotMaty(angle);
        for (int j = 0; j < step_c; j++) {
            Vector4f rotated =   rot * tr * ref[j];
            torus[i*step_c + j] = rotated;
        }
    }
    return torus;
}



///////////////////////////////////////



//perspective mætrix, the same one as in opengl
Matrix4f perspectiveMatrix(float FOV, float aspect, float near, float far) {
    float t = near * tan(FOV/2);
    float b = -t;
    float r = aspect*t;
    float g = -r;

    Matrix4f perspective;
    perspective << 2*near/(r-g), 0.0f,         (r+g)/(r-g),     0.0f,
                   0.0f,         2*near/(t-b), (t+b)/(t-b),     0.0f,
                   0.0f,         0.0f, -(far+near)/(far-near),  -2*far*near/(far-near),
                   0.0f,         0.0f,         -1.0f,           0.0f;
                         
    return perspective;
}

//the transformation applied to the camera
//initialized as the identity 
Matrix4f viewMatrix(float tx, float ty, float tz) {
    Matrix4f view;
    view << 1.0f, 0.0f, 0.0f, tx,
            0.0f, 1.0f, 0.0f, ty,
            0.0f, 0.0f, -1.0f, tz,
            0.0f, 0.0f, 0.0f, 1.0f;

    return view;
}

//update the model (torus here) to camera view
Vector4f* PVM(Vector4f* model, Matrix4f viewMatrix, Matrix4f perspective) {
    static Vector4f newModel[modSize];
    for (int i = 0 ; i < modSize ; i++) {
        newModel[i] = perspective * viewMatrix * model[i];
    }
    return newModel;
}

//////////




void cartesianCoodinates(Vector4f* vec) {
    float norm;
    for (int i = 0 ; i < modSize ; i++) {
        vec[i] /= vec[i](3);
        vec[i](0) *= width;
        vec[i](1) *= height;
    }
}


//we can create a transformation mætrix with the function above and pass them to this function
//and apply them to the camera
void inputView(Matrix4f transform, Matrix4f& view) {
    view = transform * view;
}






int main() {
    //torus
    Vector4f* torus = create_torus(0, 0, 0, 20, 100);

    //camera
    float FOV = 90*M_PI/(180);
    float aspect = 4/3;
    float near = 0.1;
    float far = 500;

    //beginning cartesian coordinates of the camera
    float tx = 0;
    float ty = 0;
    float tz = 30;


    Matrix4f perspective = perspectiveMatrix(FOV, aspect, near, far);
    Matrix4f view = viewMatrix(tx, ty, tz);

    Matrix4f transform;
    

    //SFML
    // create the window (remember: it's safer to create it in the main thread due to OS limitations)
    sf::RenderWindow window(sf::VideoMode(width, height), "Donut");
    sf::CircleShape vertex[modSize];
    
     // run the program as long as the window is open
    while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            //keys to move camera
            //we move following the camera direction
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) inputView(translateMatrix(-step*view(0,0), -step*view(0,1), -step*view(0,2)), view);            
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) inputView(translateMatrix(step*view(0,0), step*view(0,1), step*view(0,2)), view);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) inputView(translateMatrix(-step*view(1,0), -step*view(1,1), -step*view(1,2)), view);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) inputView(translateMatrix(step*view(1,0), step*view(1,1), step*view(1,2)), view);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) inputView(translateMatrix(-step*view(2,0), -step*view(2,1), -step*view(2,2)), view);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) inputView(translateMatrix(step*view(2,0), step*view(2,1), step*view(2,2)), view);
            //rotations of the camera
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) inputView(rotMatx(angle_step), view);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) inputView(rotMatx(-angle_step), view);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) inputView(rotMaty(-angle_step), view);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) inputView(rotMaty(angle_step), view);
            
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed) window.close();

        }

        //compute the torus in camera view
        Vector4f* camTorus = PVM(torus, view, perspective);
        cartesianCoodinates(camTorus);



        //just some prints to verify camera coordinate 
        for (int i = 0 ; i < modSize ; i++) {
            cout << camTorus[i](0) << " " << camTorus[i](1) << " " << camTorus[i](2) << " " << camTorus[i](3) << endl;
        }
        cout << "camera " << view(0,3) << " " << view(1,3) << " " << view(2,3) << " " << view(3,3) << endl;
        

        //set the position of the torus following what we computed
        for (int i = 0 ; i < modSize ; i++) {
            vertex[i].setPosition(sf::Vector2f(camTorus[i](0), camTorus[i](1)));
            vertex[i].setFillColor(sf::Color(255, 255, 255));
            vertex[i].setRadius(1);
        }
 

        // clear the window with black color
        window.clear(sf::Color::Black);

        // draw if the donut is in our field of view (on the z-axis)
        for (int i = 0 ; i < modSize ; i++) {
            if (camTorus[i](2) > 0 && camTorus[i](2) < 1) window.draw(vertex[i]);
        }

        // end the current frame
        window.display();
    }

    return 0;
}