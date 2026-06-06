#include "g32906/m32906.h"
QVector<double> m32906::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
