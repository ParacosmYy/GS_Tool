#include "g32106/m32106.h"
QVector<double> m32106::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
