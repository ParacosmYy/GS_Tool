#include "o35514/m35514.h"
QVector<double> m35514::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
