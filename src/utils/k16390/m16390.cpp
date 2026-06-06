#include "k16390/m16390.h"
QVector<double> m16390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
