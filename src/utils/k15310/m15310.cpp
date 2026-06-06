#include "k15310/m15310.h"
QVector<double> m15310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
