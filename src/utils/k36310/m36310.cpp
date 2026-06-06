#include "k36310/m36310.h"
QVector<double> m36310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
