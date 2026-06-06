#include "m30812/m30812.h"
QVector<double> m30812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
