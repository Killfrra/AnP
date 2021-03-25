#ifndef QUOTES_ENGINE
#define QUOTES_ENGINE

#include <stdio.h>

#define roll(i, len) (i = (i + 1) % len)
#define len(x)  (sizeof(x) / sizeof((x)[0]))

typedef struct {
    int exit, remove, mistake, start, edit;
} QuotesState;

QuotesState quotes_state = {};

const char * const start_quotes[] = {
    "� �����, � ������ � ����. ������ �� ����� ����",
    "��� ������ ������ ������ �����. � �� ������ ���������",
    "��� ��� ��� ����. ��� ������ ����. ������� �������� �������",
    "���. ������ ��� ���. ���. ���� ��� ���� �����",
    "����. �������� ������� ���. ���� ����� ����!",
    "��������� ����� ����. ����� ������ � ����. ���� �������, ������ ���. ����� ���� �����!",
    "����������� ����. ������ ��� ��������. ��� ��� �� ������, ����� ���� �����!",
    "��������� ������� �����, ������������ �����. ����� ������ ��� ��� �������� � �����",
    "..."
};

const char * const mistake_quotes[] = {
    "��� ��� ����� � ���, ���� � ������� � ������ ���!",
    "��� ��� ���� ��� ������� ���� ������� ���������� ����!",
    "����! ���� �� ���� � ����",
    "����! ���� � ���� ����",
    "�������, � �� ���� - ������� � �� ���� ������",
    "�������! ������. ������ ���. �� ������ �������� ����",
    "�� ����� ����, ������ ���, �������� � ���� �����. ���� ������, ��������� ���� � ��������� ����!",
    "�� �� ���������, �� ������ ������ � ����� �������� ������",
    "����, ������ �� �����, ���� �� ��� ���� ����� ������",
    "��, �����, ������ ���� � ���� ���� ������� ��� �����",
    "..."
};

const char * const edit_quotes[] = {
    "�� ������ ����� � ������� ����. �� ����������� ����, ����� �� �����",
    "�� ������ �����. ����� �������. ��� ����� ���� �� �� ����� �����",
    "�� ���������� � ���������� ������: ��� ��� ������, ���� ������ � �����",
    "���������������"
};

const char * const remove_quotes[] = {
    "���� � ������. � ����� ���� ���� �����",
    "���� � ������. ������ ��� ��������� ������",
    "�� ����� �� ������ ������� ������� ���",
    "����� �� ������ ������� �� �������� ����� �����, ��� ����� ����� ������� ���",
    "������ ��������� ������ � ������� ����������� ����� ����� �����",
    "� ��������� ������� ���� ������� ����� �����. �� �� ������� ��� �����",
    "����� ���� ��� ������, ������ �������� ��������",
    "��� ������ �������� � ���������� ����� � ��, � ����� �����",
    "������. ��� ����� ������, ��� ���������� ����� �� ����",
    "����� � ���������� - ���� ����� �����. ����� � �����, ����� ����� �����",
    "�� �� ������ - ��� � �����. � ���� ������ ���� ����",
    "������� �������� �� ���� ������ ����� �� ����",
    "������ ������ ����� � ���. ��� ����� ������ ����",
    "���. � ������ �� ����. ���� ��� �����? ��-��-��-��",
    "� ��� ���������. ��� ������ � ���� ������ � �������",
    "�� ��������� � ���� ������ ������ ������",
    "��, �� ��� �� ������ ������, ��������� ����� ���������� ������ ������. ������ ���� �������� ��!",
    "���� ���� ������� ������. � ����� �� ������� ������, ������ � �� ���"
};

const char * const exit_quotes[] = {
    "� ������� � ���� ��� ������ � ����� ���� � ������� ����",
    "����� � �� �����������. � �� ����������� �� ���",
    "�� ������ ������� �� ������",
    "���� � ������. � ����, �� ��� � ����� �� �����",
    "���� � ������. �� ��� ��� ���� ������ �����",
    "�� ����� � ������ � ����. �� ����� ��� � ��� �� �����",
    "�� � ����. � ��� �������. �� ������, ��� ���� ������",
    "� ����� ���� ������ �������, ��� ���� ���� �����",
    "����� �������� ������� ���� �� ������ ��� � ����� �� �����, �� ����� ����",
    "������ �� � ��� �����. ���� ������ �� ������ ������. �� ������ ���� �����. ����� � ��� �� ������",
    "������ ��� � �������� ����. ������ ������ �� ���� � ���� ����� ����",
    "���� �������� �������. �������� � ������� �������. � �����, ������� �����",
    "���������� ��� � ������� ��� �����. � ����� ����� ������ ���� ���� � ���",
    "� ����� ������ � �� ���� � ������. � � ����� ��������. ����� � ��� ����?",
    "����-���� �� �������, ������ ���� � �����",
    "������ �������! �� ����� ������ �� ����",
    "������ �������! �� � ��� ���� �� ������",
    "���� ����� ���� �� �����, �� �� ������ ������ - ����� ������ �����, ����� ���� �����!",
};

#define event_quote(event) \
void event##_quote(){ \
    setCursorPosition(1, 1); \
    printf("%s (c) ����", event##_quotes[quotes_state.event]); \
    roll(quotes_state.event, len(event##_quotes)); \
}

event_quote(start)
event_quote(exit)
event_quote(edit)
event_quote(mistake)
event_quote(remove)

#endif