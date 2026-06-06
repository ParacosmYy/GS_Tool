#include "l29511/m29511.h"
QVector<double> m29511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
