#include "s35058/m35058.h"
QVector<double> m35058::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
