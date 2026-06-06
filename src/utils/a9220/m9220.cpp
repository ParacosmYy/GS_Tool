#include "a9220/m9220.h"
QVector<double> m9220::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
