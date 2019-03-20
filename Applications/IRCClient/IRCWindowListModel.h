#pragma once

#include <LibGUI/GTableModel.h>
#include <AK/Function.h>

class IRCClient;
class IRCWindow;

class IRCWindowListModel final : public GTableModel {
public:
    enum Column {
        Name,
    };

    static Retained<IRCWindowListModel> create(IRCClient& client) { return adopt(*new IRCWindowListModel(client)); }
    virtual ~IRCWindowListModel() override;

    virtual int row_count() const override;
    virtual int column_count() const override;
    virtual String column_name(int column) const override;
    virtual ColumnMetadata column_metadata(int column) const override;
    virtual GVariant data(const GModelIndex&, Role = Role::Display) const override;
    virtual void update() override;
    virtual void activate(const GModelIndex&) override;

    Function<void(IRCWindow&)> on_activation;

private:
    explicit IRCWindowListModel(IRCClient&);

    IRCClient& m_client;
};
