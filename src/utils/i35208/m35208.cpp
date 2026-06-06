#include "i35208/m35208.h"
QVector<double> m35208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
