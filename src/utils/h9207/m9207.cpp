#include "h9207/m9207.h"
QVector<double> m9207::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
