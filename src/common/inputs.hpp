#include <vector>
#include "Constants.hpp"

vector<int *> *get_default()
{
    vector<int *> *vec = new vector<int *>();

    for (int i = 0; i < CHUNK_SIZE; i++)
    {
        vec->push_back(new int(4));
    }

    for (int i = 0; i < CHUNK_SIZE; i++)
    {
        vec->push_back(new int(1));
    }

    for (int i = 0; i < CHUNK_SIZE; i++)
    {
        vec->push_back(new int(8));
    }

    return vec;
}

vector<int *> *get_constant()
{
    vector<int *> *vec = new vector<int *>();

    for (int i = 0; i < (CHUNK_SIZE * 3); i++)
    {
        vec->push_back(new int(4));
    }

    return vec;
}

vector<int *> *get_reverse_default()
{
    vector<int *> *vec = new vector<int *>();

    for (int i = 0; i < CHUNK_SIZE; i++)
    {
        vec->push_back(new int(8));
    }

    for (int i = 0; i < CHUNK_SIZE; i++)
    {
        vec->push_back(new int(1));
    }

    for (int i = 0; i < CHUNK_SIZE; i++)
    {
        vec->push_back(new int(4));
    }

    return vec;
}

vector<int *> *get_lowhigh()
{
    int chunk = (CHUNK_SIZE * 3) / 2;
    vector<int *> *vec = new vector<int *>();

    for (int i = 0; i < chunk; i++)
    {
        vec->push_back(new int(1));
    }

    for (int i = 0; i < chunk; i++)
    {
        vec->push_back(new int(8));
    }

    return vec;
}

vector<int *> *get_highlow()
{
    int chunk = (CHUNK_SIZE * 3) / 2;
    vector<int *> *vec = new vector<int *>();

    for (int i = 0; i < chunk; i++)
    {
        vec->push_back(new int(8));
    }

    for (int i = 0; i < chunk; i++)
    {
        vec->push_back(new int(1));
    }

    return vec;
}