#include "a9340/m9340.h"
QVector<double> m9340::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
