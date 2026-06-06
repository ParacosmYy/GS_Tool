#include "a16140/m16140.h"
QVector<double> m16140::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
