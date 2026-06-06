#include "b35541/m35541.h"
QVector<double> m35541::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
