#include "k10310/m10310.h"
QVector<double> m10310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
