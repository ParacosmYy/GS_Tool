#include "a8080/m8080.h"
QVector<double> m8080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
