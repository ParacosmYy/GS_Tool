#include "a19300/m19300.h"
QVector<double> m19300::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
