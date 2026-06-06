#include "j28009/m28009.h"
QVector<double> m28009::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
