#include "a35000/m35000.h"
QVector<double> m35000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
