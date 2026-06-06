#include "m35792/m35792.h"
QVector<double> m35792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
