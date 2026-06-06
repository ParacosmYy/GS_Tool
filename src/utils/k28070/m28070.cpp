#include "k28070/m28070.h"
QVector<double> m28070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
