#include "l10911/m10911.h"
QVector<double> m10911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
