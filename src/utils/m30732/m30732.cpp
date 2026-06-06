#include "m30732/m30732.h"
QVector<double> m30732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
