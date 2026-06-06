#include "l35731/m35731.h"
QVector<double> m35731::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
