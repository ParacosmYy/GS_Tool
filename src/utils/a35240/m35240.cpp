#include "a35240/m35240.h"
QVector<double> m35240::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
