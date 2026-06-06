#include "k9310/m9310.h"
QVector<double> m9310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
