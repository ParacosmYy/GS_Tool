#include "a24220/m24220.h"
QVector<double> m24220::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
