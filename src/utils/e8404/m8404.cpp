#include "e8404/m8404.h"
QVector<double> m8404::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
