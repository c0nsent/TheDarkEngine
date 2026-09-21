#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

int main()
{
    glm::vec4 vec{1.f, 0.0, 0.0f, 1.0f};
    glm::mat4 trans{1.f};

    trans = glm::translate(trans, glm::vec3(1.f, 1.f, 0.f));
    vec = trans * vec;

    std::cout << vec.x << vec.y << vec.z << std::endl;
}