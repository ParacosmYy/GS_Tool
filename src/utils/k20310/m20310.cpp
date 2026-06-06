#include "k20310/m20310.h"
QVector<double> m20310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
