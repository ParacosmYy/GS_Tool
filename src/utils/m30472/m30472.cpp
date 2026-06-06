#include "m30472/m30472.h"
QVector<double> m30472::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
