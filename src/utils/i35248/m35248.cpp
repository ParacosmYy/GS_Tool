#include "i35248/m35248.h"
QVector<double> m35248::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
