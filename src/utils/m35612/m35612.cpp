#include "m35612/m35612.h"
QVector<double> m35612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
