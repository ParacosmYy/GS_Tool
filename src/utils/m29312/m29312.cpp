#include "m29312/m29312.h"
QVector<double> m29312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
