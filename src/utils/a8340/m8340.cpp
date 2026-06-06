#include "a8340/m8340.h"
QVector<double> m8340::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
