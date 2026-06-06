#include "a8920/m8920.h"
QVector<double> m8920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
