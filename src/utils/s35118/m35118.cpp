#include "s35118/m35118.h"
QVector<double> m35118::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
