#include "e35004/m35004.h"
QVector<double> m35004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
