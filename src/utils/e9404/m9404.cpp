#include "e9404/m9404.h"
QVector<double> m9404::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
