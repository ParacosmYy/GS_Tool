#include "s35038/m35038.h"
QVector<double> m35038::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
