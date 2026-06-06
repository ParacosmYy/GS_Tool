#include "m35872/m35872.h"
QVector<double> m35872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
