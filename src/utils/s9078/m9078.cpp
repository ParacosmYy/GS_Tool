#include "s9078/m9078.h"
QVector<double> m9078::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
