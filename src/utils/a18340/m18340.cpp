#include "a18340/m18340.h"
QVector<double> m18340::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
