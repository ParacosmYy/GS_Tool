#include "p16715/m16715.h"
QVector<double> m16715::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
