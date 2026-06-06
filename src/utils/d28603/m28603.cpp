#include "d28603/m28603.h"
QVector<double> m28603::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
