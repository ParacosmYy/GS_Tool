#include "i30208/m30208.h"
QVector<double> m30208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
