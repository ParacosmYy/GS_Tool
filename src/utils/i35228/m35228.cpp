#include "i35228/m35228.h"
QVector<double> m35228::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
