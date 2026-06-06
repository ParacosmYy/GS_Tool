#include "j18709/m18709.h"
QVector<double> m18709::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
