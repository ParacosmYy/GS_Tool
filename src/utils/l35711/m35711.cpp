#include "l35711/m35711.h"
QVector<double> m35711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
