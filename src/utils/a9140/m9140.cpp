#include "a9140/m9140.h"
QVector<double> m9140::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
