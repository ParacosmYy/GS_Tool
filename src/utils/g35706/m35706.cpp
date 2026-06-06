#include "g35706/m35706.h"
QVector<double> m35706::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
