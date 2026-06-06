#include "i35328/m35328.h"
QVector<double> m35328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
